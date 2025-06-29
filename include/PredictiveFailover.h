#pragma once

#include <vector>
#include <memory>
#include <queue>
#include <unordered_map>
#include <Eigen/Dense>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

namespace ClusterForge {

class Node;
class Cluster;

struct TimeSeriesData {
    std::vector<double> cpu_usage;
    std::vector<double> memory_usage;
    std::vector<double> disk_io;
    std::vector<double> network_io;
    std::vector<double> temperature;
    std::vector<double> power_consumption;
    std::chrono::system_clock::time_point timestamp;
    
    TimeSeriesData() {}
};

struct AnomalyScore {
    double cpu_anomaly;
    double memory_anomaly;
    double disk_anomaly;
    double network_anomaly;
    double temperature_anomaly;
    double power_anomaly;
    double combined_score;
    double confidence;
    
    AnomalyScore() : cpu_anomaly(0.0), memory_anomaly(0.0), disk_anomaly(0.0),
                     network_anomaly(0.0), temperature_anomaly(0.0), power_anomaly(0.0),
                     combined_score(0.0), confidence(0.0) {}
};

struct FailurePrediction {
    int node_id;
    double failure_probability;
    std::chrono::system_clock::time_point predicted_failure_time;
    std::vector<AnomalyScore> anomaly_history;
    std::string failure_reason;
    double confidence;
    
    FailurePrediction() : node_id(-1), failure_probability(0.0), confidence(0.0) {}
};

struct MigrationPlan {
    int source_node_id;
    std::vector<int> target_node_ids;
    std::vector<int> task_ids_to_migrate;
    double migration_priority;
    std::chrono::system_clock::time_point planned_migration_time;
    std::string migration_strategy;
    
    MigrationPlan() : source_node_id(-1), migration_priority(0.0) {}
};

class LSTMCell {
private:
    Eigen::MatrixXd Wf_, Wi_, Wc_, Wo_;  // Weight matrices
    Eigen::VectorXd bf_, bi_, bc_, bo_;  // Bias vectors
    Eigen::VectorXd ft_, it_, ct_, ot_;  // Gate activations
    Eigen::VectorXd c_t_, h_t_;          // Cell and hidden states
    
    int input_size_;
    int hidden_size_;

public:
    LSTMCell(int input_size, int hidden_size);
    
    Eigen::VectorXd forward(const Eigen::VectorXd& input, const Eigen::VectorXd& h_prev, 
                           const Eigen::VectorXd& c_prev);
    void backward(const Eigen::VectorXd& dh_next, const Eigen::VectorXd& dc_next);
    void updateWeights(double learning_rate);
    
    Eigen::VectorXd getHiddenState() const { return h_t_; }
    Eigen::VectorXd getCellState() const { return c_t_; }
};

class LSTMNetwork {
private:
    std::vector<LSTMCell> lstm_layers_;
    Eigen::MatrixXd output_weights_;
    Eigen::VectorXd output_bias_;
    
    int input_size_;
    int hidden_size_;
    int num_layers_;
    int output_size_;
    double learning_rate_;

public:
    LSTMNetwork(int input_size, int hidden_size, int num_layers, int output_size, double lr = 0.001);
    
    Eigen::VectorXd forward(const std::vector<Eigen::VectorXd>& input_sequence);
    void backward(const Eigen::VectorXd& target);
    void updateWeights();
    void saveModel(const std::string& filename);
    void loadModel(const std::string& filename);
    
    // Sequence prediction
    std::vector<Eigen::VectorXd> predictSequence(const std::vector<Eigen::VectorXd>& input_sequence, int steps);
};

class AnomalyDetector {
private:
    LSTMNetwork lstm_model_;
    std::vector<TimeSeriesData> training_data_;
    std::vector<double> reconstruction_errors_;
    
    double anomaly_threshold_;
    int sequence_length_;
    int prediction_horizon_;

public:
    AnomalyDetector(int input_size, int hidden_size, int num_layers, double threshold = 0.1);
    
    AnomalyScore detectAnomaly(const TimeSeriesData& current_data);
    std::vector<AnomalyScore> detectAnomalies(const std::vector<TimeSeriesData>& data_sequence);
    void trainModel(const std::vector<TimeSeriesData>& training_data);
    void updateThreshold(double threshold) { anomaly_threshold_ = threshold; }
    
    double getReconstructionError() const;
    void saveModel(const std::string& filename);
    void loadModel(const std::string& filename);
};

class FailurePredictor {
private:
    AnomalyDetector anomaly_detector_;
    std::unordered_map<int, std::vector<TimeSeriesData>> node_data_;
    std::unordered_map<int, std::vector<AnomalyScore>> node_anomalies_;
    std::unordered_map<int, FailurePrediction> failure_predictions_;
    
    double prediction_horizon_hours_;
    double confidence_threshold_;
    int min_anomaly_count_;

public:
    FailurePredictor(int input_size, int hidden_size, int num_layers);
    
    FailurePrediction predictFailure(int node_id);
    std::vector<FailurePrediction> predictAllFailures();
    void updateNodeData(int node_id, const TimeSeriesData& data);
    void trainPredictor(const std::unordered_map<int, std::vector<TimeSeriesData>>& training_data);
    
    double getPredictionAccuracy() const;
    void saveModel(const std::string& filename);
    void loadModel(const std::string& filename);
};

class PredictiveFailover {
private:
    std::shared_ptr<Cluster> cluster_;
    FailurePredictor failure_predictor_;
    
    std::queue<MigrationPlan> migration_queue_;
    std::unordered_map<int, MigrationPlan> active_migrations_;
    
    boost::asio::io_context io_context_;
    boost::thread failover_thread_;
    std::atomic<bool> is_running_;
    
    // Configuration
    double prediction_threshold_;
    double migration_threshold_;
    int migration_timeout_ms_;
    bool enable_proactive_migration_;
    
    // Performance tracking
    std::vector<FailurePrediction> successful_predictions_;
    std::vector<FailurePrediction> failed_predictions_;
    int total_migrations_;
    int successful_migrations_;

public:
    explicit PredictiveFailover(std::shared_ptr<Cluster> cluster);
    ~PredictiveFailover();
    
    // Core failover functionality
    void start();
    void stop();
    bool isRunning() const { return is_running_; }
    
    // Prediction and monitoring
    void monitorNodes();
    std::vector<FailurePrediction> getFailurePredictions();
    void updateNodeMetrics(int node_id, const ResourceMetrics& metrics);
    
    // Migration management
    MigrationPlan createMigrationPlan(int source_node_id);
    bool executeMigration(const MigrationPlan& plan);
    void cancelMigration(int source_node_id);
    std::vector<MigrationPlan> getPendingMigrations() const;
    
    // Proactive migration
    void initiateProactiveMigration(int node_id);
    bool shouldMigrateProactively(int node_id) const;
    std::vector<int> selectMigrationTargets(int source_node_id, const std::vector<int>& task_ids);
    
    // Configuration
    void setPredictionThreshold(double threshold) { prediction_threshold_ = threshold; }
    void setMigrationThreshold(double threshold) { migration_threshold_ = threshold; }
    void setMigrationTimeout(int timeout_ms) { migration_timeout_ms_ = timeout_ms; }
    void enableProactiveMigration(bool enable) { enable_proactive_migration_ = enable; }
    
    // Performance monitoring
    double getPredictionAccuracy() const;
    double getMigrationSuccessRate() const;
    int getTotalMigrations() const { return total_migrations_; }
    int getSuccessfulMigrations() const { return successful_migrations_; }
    
    // Model management
    void saveModels(const std::string& base_filename);
    void loadModels(const std::string& base_filename);
    void retrainModels(const std::unordered_map<int, std::vector<TimeSeriesData>>& training_data);
    
    // Utility
    std::string getStatusReport() const;
    void clearHistory();

private:
    void failoverLoop();
    void processMigrationQueue();
    void handleNodeFailure(int node_id);
    void updateFailurePredictions();
    TimeSeriesData createTimeSeriesData(int node_id);
    double calculateMigrationPriority(const MigrationPlan& plan);
    void logMigration(const MigrationPlan& plan, bool success);
};

} // namespace ClusterForge 